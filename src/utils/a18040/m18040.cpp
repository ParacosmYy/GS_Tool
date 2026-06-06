#include "a18040/m18040.h"
QVector<double> m18040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
