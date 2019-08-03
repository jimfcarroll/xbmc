
struct Runnable
{
  std::shared_ptr<Runnable>  this_ref;

  inline virtual ~Runnable() = default;

  virtual void Run() = 0;
};

template<typename Callable> struct RunnableImpl : public Runnable
{
  Callable   func;

  RunnableImpl(Callable&& f) : func(std::forward<Callable>(f))
  { }

  void Run() { func(); }
};

template<typename Callable, typename R> struct CallableImpl : public Runnable
{
  Callable   func;
  std::promise<R> promise;

  CallableImpl(Callable&& f) : func(std::forward<Callable>(f))
  { }

  void Run() {
    promise.set_value(func());
  }

  std::future<R> future() {
    return promise.get_future();
  }
};

template<typename Callable> std::shared_ptr<RunnableImpl<Callable>>
makeRunnableImplRef(Callable&& __f) {
  return std::make_shared<RunnableImpl<Callable> >(std::forward<Callable>(__f));
}

template<typename Result> class AssignPromise {
public:
  template<typename Callable>
  static std::shared_ptr<CallableImpl<Callable, Result>> assignPromise(Callable&& __f) {
    return std::make_shared<CallableImpl<Callable, Result> >(std::forward<Callable>(__f));
  }
};

