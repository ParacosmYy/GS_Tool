#include "a29360/m29360.h"
QVector<double> m29360::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
