#include "a32040/m32040.h"
QVector<double> m32040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
