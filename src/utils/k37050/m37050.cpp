#include "k37050/m37050.h"
QVector<double> m37050::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
