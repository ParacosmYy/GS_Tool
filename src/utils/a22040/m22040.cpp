#include "a22040/m22040.h"
QVector<double> m22040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
