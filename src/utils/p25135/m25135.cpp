#include "p25135/m25135.h"
QVector<double> m25135::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
