#include "f35185/m35185.h"
QVector<double> m35185::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
