#include "a9040/m9040.h"
QVector<double> m9040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
