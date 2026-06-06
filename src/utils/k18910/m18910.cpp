#include "k18910/m18910.h"
QVector<double> m18910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
