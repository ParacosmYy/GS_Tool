#include "a24040/m24040.h"
QVector<double> m24040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
