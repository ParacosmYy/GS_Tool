#include "a15040/m15040.h"
QVector<double> m15040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
