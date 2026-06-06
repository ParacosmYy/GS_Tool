#include "c25102/m25102.h"
QVector<double> m25102::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
