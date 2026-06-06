#include "b15081/m15081.h"
QVector<double> m15081::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
