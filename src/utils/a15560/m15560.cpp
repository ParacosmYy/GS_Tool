#include "a15560/m15560.h"
QVector<double> m15560::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
