#include "k34330/m34330.h"
QVector<double> m34330::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
