#include "i20208/m20208.h"
QVector<double> m20208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
