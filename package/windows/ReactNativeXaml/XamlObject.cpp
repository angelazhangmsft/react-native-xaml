#include "pch.h"
#include "XamlObject.h"
#include "../include/Shared/cdebug.h"

namespace jsi = facebook::jsi;

template <typename TLambda, std::enable_if_t<!std::is_void<std::invoke_result_t<TLambda>>::value, int> = 0>
auto XamlObject::RunOnUIThread(const TLambda& code) const {
  std::condition_variable cv;
  std::mutex mutex;
  std::optional<decltype(code())> result{};
  m_metadata->UIDispatcher().Post([&]() {
    try {
      result = code();
    }
    catch (const winrt::hresult& h) {
      OutputDebugStringA(std::to_string(h.value).c_str());
      OutputDebugStringA("\n");
    }
    cv.notify_all();
    });

  std::unique_lock<std::mutex> lock(mutex);
  cv.wait(lock);
  assert(result.has_value());
  return result.value();
}

template <typename TLambda, std::enable_if_t<std::is_void<std::invoke_result_t<TLambda>>::value, int> = 0>
void RunOnUIThread(const TLambda& code) {
  std::condition_variable cv;
  std::mutex mutex;
  m_metadata->UIDispatcher().Post([&]() {
    code();
    cv.notify_all();
    });

  std::unique_lock<std::mutex> lock(mutex);
  cv.wait(lock);
}

jsi::Value XamlObject::get(jsi::Runtime& rt, const jsi::PropNameID& nameId) noexcept {
  try {
    auto name = nameId.utf8(rt);
    if (name == "className") {
      auto cn = winrt::to_string(winrt::get_class_name(m_obj));
      return jsi::String::createFromUtf8(rt, cn);
    }
    else {
      auto res = RunOnUIThread([&]() {
        if (auto prop = m_metadata->GetProp(name, m_obj)) {
          if (prop->xamlPropertyGetter) {
            auto dp = prop->xamlPropertyGetter();
            if (auto dobj = m_obj.try_as<DependencyObject>()) {
              auto val = dobj.GetValue(dp);
              return val;
            }
          }
        }
        return winrt::Windows::Foundation::IInspectable{ nullptr };
        });
      return IInspectableToValue(rt, res);
    }
  }
  catch (const winrt::hresult& hr) {
    OutputDebugStringA(std::to_string(hr.value).c_str());
    OutputDebugStringA("\n");
  }
  return jsi::HostObject::get(rt, nameId);
}

facebook::jsi::Value XamlObject::IInspectableToValue(jsi::Runtime& rt, const winrt::Windows::Foundation::IInspectable& o) const {
  if (o == nullptr) return jsi::Value::null();
  else if (auto refStr = o.try_as<winrt::Windows::Foundation::IReference<winrt::hstring>>()) {
    auto str = winrt::unbox_value<winrt::hstring>(o);
    return jsi::String::createFromUtf8(rt, winrt::to_string(str));
  }
  else if (auto refInt = o.try_as<winrt::Windows::Foundation::IReference<int32_t>>()) {
    return jsi::Value(refInt.GetInt32());
  }
  else if (auto refInt64 = o.try_as<winrt::Windows::Foundation::IReference<int64_t>>()) {
    auto v = refInt64.GetInt64();
    assert((v < std::numeric_limits<int32_t>::max()) && (v > std::numeric_limits<int32_t>::min()));
    return jsi::Value(refInt64.GetInt32());
  }
  else if (auto refDouble = o.try_as<winrt::Windows::Foundation::IReference<double>>()) {
    return jsi::Value(refDouble.GetDouble());
  }
  else if (auto refBool = o.try_as<winrt::Windows::Foundation::IReference<bool>>()) {
    return jsi::Value(refBool.GetBoolean());
  }
  else if (auto ref = o.try_as<winrt::Windows::Foundation::IReference<xaml::Thickness>>()) {
    auto thickness = winrt::unbox_value<Thickness>(o);
    auto t = jsi::Object(rt);
    t.setProperty(rt, "left", thickness.Left);
    t.setProperty(rt, "top", thickness.Top);
    t.setProperty(rt, "right", thickness.Right);
    t.setProperty(rt, "bottom", thickness.Bottom);
    return t;
  }
  else if (auto scb = o.try_as<xaml::Media::SolidColorBrush>()) {    
    auto name = RunOnUIThread([&]() {
      auto color = scb.Color();
      return winrt::Windows::UI::ColorHelper::ToDisplayName(color);
      });
    return jsi::String::createFromUtf8(rt, winrt::to_string(name));
  }
  return jsi::Object::createFromHostObject(rt, std::make_shared<XamlObject>(o, m_metadata));
}

void XamlObject::set(jsi::Runtime& rt, const jsi::PropNameID& name, const jsi::Value& value) noexcept {

}


std::vector<jsi::PropNameID> XamlObject::getPropertyNames(jsi::Runtime& rt) noexcept {
  if (m_obj == nullptr) return {};
  if (auto str = m_obj.try_as<winrt::Windows::Foundation::IReference<winrt::hstring>>()) {
    return facebook::jsi::PropNameID::names({ facebook::jsi::PropNameID::forUtf8(rt, "string") });
  }
  auto names = RunOnUIThread([this]() {
    std::vector<std::string> names = {
      "className",
    };
    m_metadata->PopulateNativeProps(names, m_obj);
    return names;
    });

  std::vector<facebook::jsi::PropNameID> pnames;
  for (const auto& e : names) {
    pnames.push_back(facebook::jsi::PropNameID::forUtf8(rt, e));
  }
  return pnames;
}

