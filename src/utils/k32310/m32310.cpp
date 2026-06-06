#include "k32310/m32310.h"
QVector<double> m32310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
