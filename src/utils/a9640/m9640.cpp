#include "a9640/m9640.h"
QVector<double> m9640::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
