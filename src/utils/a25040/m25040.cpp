#include "a25040/m25040.h"
QVector<double> m25040::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
