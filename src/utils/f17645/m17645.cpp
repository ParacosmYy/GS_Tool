#include "f17645/m17645.h"
QVector<double> m17645::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
