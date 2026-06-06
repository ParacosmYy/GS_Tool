#include "k32970/m32970.h"
QVector<double> m32970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
