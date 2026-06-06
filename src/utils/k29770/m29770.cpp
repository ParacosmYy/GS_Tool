#include "k29770/m29770.h"
QVector<double> m29770::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
