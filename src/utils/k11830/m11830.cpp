#include "k11830/m11830.h"
QVector<double> m11830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
