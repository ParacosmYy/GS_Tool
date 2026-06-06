#include "k28510/m28510.h"
QVector<double> m28510::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
