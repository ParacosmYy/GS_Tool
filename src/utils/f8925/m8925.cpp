#include "f8925/m8925.h"
QVector<double> m8925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
