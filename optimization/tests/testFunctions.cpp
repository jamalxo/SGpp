#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include <vector>

#include <sgpp/optimization/function/scalar/ComponentScalarFunction.hpp>
#include <sgpp/optimization/function/scalar/ComponentScalarFunctionGradient.hpp>
#include <sgpp/optimization/function/scalar/ComponentScalarFunctionHessian.hpp>
#include <sgpp/optimization/function/scalar/WrapperScalarFunction.hpp>
#include <sgpp/optimization/function/scalar/WrapperScalarFunctionGradient.hpp>
#include <sgpp/optimization/function/scalar/WrapperScalarFunctionHessian.hpp>
#include <sgpp/optimization/function/vector/WrapperVectorFunction.hpp>
#include <sgpp/optimization/function/vector/WrapperVectorFunctionGradient.hpp>
#include <sgpp/optimization/function/vector/WrapperVectorFunctionHessian.hpp>

#include <sgpp/optimization/tools/Printer.hpp>
#include <sgpp/optimization/tools/RandomNumberGenerator.hpp>

const bool use_double_precision =
#if USE_DOUBLE_PRECISION
  true;
#else
  false;
#endif /* USE_DOUBLE_PRECISION */

using namespace SGPP;
using namespace SGPP::optimization;

class ScalarTestFunction : public ScalarFunction {
 public:
  ScalarTestFunction(size_t d) : ScalarFunction(d) {}

  virtual ~ScalarTestFunction() override {}

  virtual SGPP::float_t eval(const base::DataVector& x) override {
    return x.sum();
  }

  virtual void clone(std::unique_ptr<ScalarFunction>& clone) const override {
    clone = std::unique_ptr<ScalarFunction>(
              new ScalarTestFunction(*this));
  }
};

class ScalarTestGradient : public ScalarFunctionGradient {
 public:
  ScalarTestGradient(size_t d) : ScalarFunctionGradient(d) {}

  virtual ~ScalarTestGradient() override {}

  virtual SGPP::float_t eval(const base::DataVector& x,
                             base::DataVector& gradient) override {
    for (size_t t = 0; t < d; t++) {
      gradient[t] = static_cast<SGPP::float_t>(t) * x[t];
    }

    return x.sum();
  }

  virtual void clone(std::unique_ptr<ScalarFunctionGradient>& clone) const
  override {
    clone = std::unique_ptr<ScalarFunctionGradient>(
              new ScalarTestGradient(*this));
  }
};

class ScalarTestHessian : public ScalarFunctionHessian {
 public:
  ScalarTestHessian(size_t d) : ScalarFunctionHessian(d) {}

  virtual ~ScalarTestHessian() override {}

  virtual SGPP::float_t eval(const base::DataVector& x,
                             base::DataVector& gradient,
                             base::DataMatrix& hessian) override {
    for (size_t t = 0; t < d; t++) {
      gradient[t] = static_cast<SGPP::float_t>(t) * x[t];

      for (size_t t2 = 0; t2 < d; t2++) {
        hessian(t, t2) = static_cast<SGPP::float_t>(t) * x[t] +
                         static_cast<SGPP::float_t>(t2) * x[t2];
      }
    }

    return x.sum();
  }

  virtual void clone(std::unique_ptr<ScalarFunctionHessian>& clone) const
  override {
    clone = std::unique_ptr<ScalarFunctionHessian>(
              new ScalarTestHessian(*this));
  }
};

class VectorTestFunction : public VectorFunction {
 public:
  VectorTestFunction(size_t d, size_t m) : VectorFunction(d, m) {}

  virtual ~VectorTestFunction() override {}

  virtual void eval(const base::DataVector& x,
                    base::DataVector& value) override {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();
    }
  }

  virtual void clone(std::unique_ptr<VectorFunction>& clone) const override {
    clone = std::unique_ptr<VectorFunction>(
              new VectorTestFunction(*this));
  }
};

class VectorTestGradient : public VectorFunctionGradient {
 public:
  VectorTestGradient(size_t d, size_t m) : VectorFunctionGradient(d, m) {}

  virtual ~VectorTestGradient() override {}

  virtual void eval(const base::DataVector& x,
                    base::DataVector& value,
                    base::DataMatrix& gradient) override {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();

      for (size_t t = 0; t < d; t++) {
        gradient(i, t) = static_cast<SGPP::float_t>(i) *
                         static_cast<SGPP::float_t>(t) * x[t];
      }
    }
  }

  virtual void clone(std::unique_ptr<VectorFunctionGradient>& clone) const
  override {
    clone = std::unique_ptr<VectorFunctionGradient>(
              new VectorTestGradient(*this));
  }
};

class VectorTestHessian : public VectorFunctionHessian {
 public:
  VectorTestHessian(size_t d, size_t m) : VectorFunctionHessian(d, m) {}

  virtual ~VectorTestHessian() override {}

  virtual void eval(const base::DataVector& x,
                    base::DataVector& value,
                    base::DataMatrix& gradient,
                    std::vector<base::DataMatrix>& hessian) override {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();

      for (size_t t = 0; t < d; t++) {
        gradient(i, t) = static_cast<SGPP::float_t>(i) *
                         static_cast<SGPP::float_t>(t) * x[t];

        for (size_t t2 = 0; t2 < d; t2++) {
          hessian[i](t, t2) = static_cast<SGPP::float_t>(i) *
                              static_cast<SGPP::float_t>(t) * x[t] *
                              static_cast<SGPP::float_t>(t2) * x[t];
        }
      }
    }
  }

  virtual void clone(std::unique_ptr<VectorFunctionHessian>& clone) const
  override {
    clone = std::unique_ptr<VectorFunctionHessian>(
              new VectorTestHessian(*this));
  }
};

BOOST_AUTO_TEST_CASE(TestComponentScalarFunction) {
  // Test SGPP::optimization::ComponentScalarFunction.
  base::DataVector x(3);
  x[0] = 0.12;
  x[1] = 0.34;
  x[2] = 0.56;

  base::DataVector y(2);
  y[0] = 0.12;
  y[1] = 0.56;

  {
    ScalarTestFunction f(3);
    ComponentScalarFunction g(f, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunction> g2;
    g.clone(g2);
    BOOST_CHECK_EQUAL(f.eval(x), g2->eval(y));

    BOOST_CHECK_THROW(ComponentScalarFunction(f, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunction(f, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }

  {
    VectorTestFunction f(3, 4);
    base::DataVector fx(4);
    f.eval(x, fx);
    ComponentScalarFunction g(f, 1, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunction> g2;
    g.clone(g2);
    BOOST_CHECK_EQUAL(fx[1], g2->eval(y));

    BOOST_CHECK_THROW(ComponentScalarFunction(f, 1, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunction(f, 1, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }
}

BOOST_AUTO_TEST_CASE(TestComponentScalarFunctionGradient) {
  // Test SGPP::optimization::ComponentScalarFunctionGradient.
  base::DataVector x(3);
  x[0] = 0.12;
  x[1] = 0.34;
  x[2] = 0.56;

  base::DataVector y(2);
  y[0] = 0.12;
  y[1] = 0.56;

  {
    ScalarTestGradient f(3);
    ComponentScalarFunctionGradient g(f, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunctionGradient> g2;
    g.clone(g2);
    base::DataVector gradF(3), gradG(2);
    BOOST_CHECK_EQUAL(f.eval(x, gradF), g2->eval(y, gradG));
    BOOST_CHECK_EQUAL(gradF[0], gradG[0]);
    BOOST_CHECK_EQUAL(gradF[2], gradG[1]);

    BOOST_CHECK_THROW(ComponentScalarFunctionGradient(f, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunctionGradient(f, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }

  {
    VectorTestGradient f(3, 4);
    base::DataVector fx(4);
    base::DataMatrix gradF(4, 3);
    f.eval(x, fx, gradF);
    ComponentScalarFunctionGradient g(f, 1, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunctionGradient> g2;
    g.clone(g2);
    base::DataVector gradG(2);
    BOOST_CHECK_EQUAL(fx[1], g2->eval(y, gradG));
    BOOST_CHECK_EQUAL(gradF(1, 0), gradG[0]);
    BOOST_CHECK_EQUAL(gradF(1, 2), gradG[1]);

    BOOST_CHECK_THROW(ComponentScalarFunctionGradient(f, 1, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunctionGradient(f, 1, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }
}

BOOST_AUTO_TEST_CASE(TestComponentScalarFunctionHessian) {
  // Test SGPP::optimization::ComponentScalarFunctionHessian.
  base::DataVector x(3);
  x[0] = 0.12;
  x[1] = 0.34;
  x[2] = 0.56;

  base::DataVector y(2);
  y[0] = 0.12;
  y[1] = 0.56;

  {
    ScalarTestHessian f(3);
    ComponentScalarFunctionHessian g(f, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunctionHessian> g2;
    g.clone(g2);
    base::DataVector gradF(3), gradG(2);
    base::DataMatrix hessF(3, 3), hessG(2, 2);
    BOOST_CHECK_EQUAL(f.eval(x, gradF, hessF), g2->eval(y, gradG, hessG));
    BOOST_CHECK_EQUAL(gradF[0], gradG[0]);
    BOOST_CHECK_EQUAL(gradF[2], gradG[1]);
    BOOST_CHECK_EQUAL(hessF(0, 0), hessG(0, 0));
    BOOST_CHECK_EQUAL(hessF(0, 2), hessG(0, 1));
    BOOST_CHECK_EQUAL(hessF(2, 0), hessG(1, 0));
    BOOST_CHECK_EQUAL(hessF(2, 2), hessG(1, 1));

    BOOST_CHECK_THROW(ComponentScalarFunctionHessian(f, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunctionHessian(f, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }

  {
    VectorTestHessian f(3, 4);
    base::DataVector fx(4);
    base::DataMatrix gradF(4, 3);
    std::vector<base::DataMatrix> hessF(4, base::DataMatrix(3, 3));
    f.eval(x, fx, gradF, hessF);
    ComponentScalarFunctionHessian g(f, 1, {NAN, 0.34, NAN});
    std::unique_ptr<ScalarFunctionHessian> g2;
    g.clone(g2);
    base::DataVector gradG(2);
    base::DataMatrix hessG(2, 2);
    BOOST_CHECK_EQUAL(fx[1], g2->eval(y, gradG, hessG));
    BOOST_CHECK_EQUAL(gradF(1, 0), gradG[0]);
    BOOST_CHECK_EQUAL(gradF(1, 2), gradG[1]);
    BOOST_CHECK_EQUAL(hessF[1](0, 0), hessG(0, 0));
    BOOST_CHECK_EQUAL(hessF[1](0, 2), hessG(0, 1));
    BOOST_CHECK_EQUAL(hessF[1](2, 0), hessG(1, 0));
    BOOST_CHECK_EQUAL(hessF[1](2, 2), hessG(1, 1));

    BOOST_CHECK_THROW(ComponentScalarFunctionHessian(f, 1, {NAN, NAN}),
                      std::runtime_error);
    BOOST_CHECK_THROW(ComponentScalarFunctionHessian(f, 1, {NAN, NAN, NAN, NAN}),
                      std::runtime_error);
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperScalarFunction) {
  // Test SGPP::optimization::TestWrapperScalarFunction.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t N = 100;
  base::DataVector x(d);
  ScalarTestFunction f1(d);
  WrapperScalarFunction f2(d, [](const base::DataVector & x) {
    return x.sum();
  });
  std::unique_ptr<ScalarFunction> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }

    BOOST_CHECK_EQUAL(f1.eval(x), f2Clone->eval(x));
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperScalarFunctionGradient) {
  // Test SGPP::optimization::TestWrapperScalarFunctionGradient.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t N = 100;
  base::DataVector x(d);
  base::DataVector gradient1(d), gradient2(d);
  ScalarTestGradient f1(d);
  WrapperScalarFunctionGradient f2(d, [](const base::DataVector & x,
  base::DataVector & gradient) {
    for (size_t t = 0; t < d; t++) {
      gradient[t] = static_cast<SGPP::float_t>(t) * x[t];
    }

    return x.sum();
  });
  std::unique_ptr<ScalarFunctionGradient> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }

    BOOST_CHECK_EQUAL(f1.eval(x, gradient1), f2Clone->eval(x, gradient2));
    BOOST_CHECK_EQUAL(gradient2.getSize(), d);

    for (size_t t = 0; t < d; t++) {
      BOOST_CHECK_EQUAL(gradient1[t], gradient2[t]);
    }
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperScalarFunctionHessian) {
  // Test SGPP::optimization::TestWrapperScalarFunctionHessian.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t N = 100;
  base::DataVector x(d);
  base::DataVector gradient1(d), gradient2(d);
  base::DataMatrix hessian1(d, d), hessian2(d, d);
  ScalarTestHessian f1(d);
  WrapperScalarFunctionHessian f2(d, [](const base::DataVector & x,
                                        base::DataVector & gradient,
  base::DataMatrix & hessian) {
    for (size_t t = 0; t < d; t++) {
      gradient[t] = static_cast<SGPP::float_t>(t) * x[t];

      for (size_t t2 = 0; t2 < d; t2++) {
        hessian(t, t2) = static_cast<SGPP::float_t>(t) * x[t] +
                         static_cast<SGPP::float_t>(t2) * x[t2];
      }
    }

    return x.sum();
  });
  std::unique_ptr<ScalarFunctionHessian> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }

    BOOST_CHECK_EQUAL(f1.eval(x, gradient1, hessian1),
                      f2Clone->eval(x, gradient2, hessian2));
    BOOST_CHECK_EQUAL(gradient2.getSize(), d);
    BOOST_CHECK_EQUAL(hessian2.getNrows(), d);
    BOOST_CHECK_EQUAL(hessian2.getNcols(), d);

    for (size_t t1 = 0; t1 < d; t1++) {
      for (size_t t2 = 0; t2 < d; t2++) {
        BOOST_CHECK_EQUAL(hessian1(t1, t2), hessian2(t1, t2));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperVectorFunction) {
  // Test SGPP::optimization::TestWrapperVectorFunction.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t m = 4;
  const size_t N = 100;
  base::DataVector x(d);
  base::DataVector value1(m), value2(m);
  VectorTestFunction f1(d, m);
  WrapperVectorFunction f2(d, m, [](const base::DataVector & x,
  base::DataVector & value) {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();
    }
  });
  std::unique_ptr<VectorFunction> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }

    f1.eval(x, value1);
    f2Clone->eval(x, value2);

    BOOST_CHECK_EQUAL(value2.getSize(), m);

    for (size_t j = 0; j < m; j++) {
      BOOST_CHECK_EQUAL(value1[j], value2[j]);
    }
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperVectorFunctionGradient) {
  // Test SGPP::optimization::TestWrapperVectorFunctionGradient.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t m = 4;
  const size_t N = 100;
  base::DataVector x(d);
  base::DataVector value1(m), value2(m);
  base::DataMatrix gradient1(m, d), gradient2(m, d);
  VectorTestGradient f1(d, m);
  WrapperVectorFunctionGradient f2(d, m, [](const base::DataVector & x,
                                   base::DataVector & value,
  base::DataMatrix & gradient) {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();

      for (size_t t = 0; t < d; t++) {
        gradient(i, t) = static_cast<SGPP::float_t>(i) *
                         static_cast<SGPP::float_t>(t) * x[t];
      }
    }
  });
  std::unique_ptr<VectorFunctionGradient> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }

    f1.eval(x, value1, gradient1);
    f2Clone->eval(x, value2, gradient2);

    BOOST_CHECK_EQUAL(value2.getSize(), m);
    BOOST_CHECK_EQUAL(gradient2.getNrows(), m);
    BOOST_CHECK_EQUAL(gradient2.getNcols(), d);

    for (size_t j = 0; j < m; j++) {
      BOOST_CHECK_EQUAL(value1[j], value2[j]);

      for (size_t t = 0; t < d; t++) {
        BOOST_CHECK_EQUAL(gradient1(j, t), gradient2(j, t));
      }
    }
  }
}

BOOST_AUTO_TEST_CASE(TestWrapperVectorFunctionHessian) {
  // Test SGPP::optimization::TestWrapperVectorFunctionHessian.
  RandomNumberGenerator::getInstance().setSeed(42);

  const size_t d = 3;
  const size_t m = 4;
  const size_t N = 100;
  base::DataVector x(d);
  base::DataVector value1(m), value2(m);
  base::DataMatrix gradient1(m, d), gradient2(m, d);
  std::vector<base::DataMatrix> hessian1(m, base::DataMatrix(d, d)),
      hessian2(m, base::DataMatrix(d, d));
  VectorTestHessian f1(d, m);
  WrapperVectorFunctionHessian f2(d, m, [](const base::DataVector & x,
                                  base::DataVector & value,
                                  base::DataMatrix & gradient,
  std::vector<base::DataMatrix>& hessian) {
    for (size_t i = 0; i < m; i++) {
      value[i] = static_cast<SGPP::float_t>(i) * x.sum();

      for (size_t t = 0; t < d; t++) {
        gradient(i, t) = static_cast<SGPP::float_t>(i) *
                         static_cast<SGPP::float_t>(t) * x[t];

        for (size_t t2 = 0; t2 < d; t2++) {
          hessian[i](t, t2) = static_cast<SGPP::float_t>(i) *
                              static_cast<SGPP::float_t>(t) * x[t] *
                              static_cast<SGPP::float_t>(t2) * x[t];
        }
      }
    }
  });
  std::unique_ptr<VectorFunctionHessian> f2Clone;
  f2.clone(f2Clone);

  for (size_t i = 0; i < N; i++) {
    for (size_t t = 0; t < d; t++) {
      x[t] = RandomNumberGenerator::getInstance().getUniformRN();
    }


    f1.eval(x, value1, gradient1, hessian1);
    f2Clone->eval(x, value2, gradient2, hessian2);

    BOOST_CHECK_EQUAL(value2.getSize(), m);
    BOOST_CHECK_EQUAL(gradient2.getNrows(), m);
    BOOST_CHECK_EQUAL(gradient2.getNcols(), d);
    BOOST_CHECK_EQUAL(hessian2.size(), m);

    for (size_t j = 0; j < m; j++) {
      BOOST_CHECK_EQUAL(value1[j], value2[j]);
      BOOST_CHECK_EQUAL(hessian2[j].getNrows(), d);
      BOOST_CHECK_EQUAL(hessian2[j].getNcols(), d);

      for (size_t t = 0; t < d; t++) {
        BOOST_CHECK_EQUAL(gradient1(j, t), gradient2(j, t));

        for (size_t t2 = 0; t2 < d; t2++) {
          BOOST_CHECK_EQUAL(hessian1[j](t, t2), hessian2[j](t, t2));
        }
      }
    }
  }
}
