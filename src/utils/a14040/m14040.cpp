#include "a14040/m14040.h"
QVector<double> m14040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
