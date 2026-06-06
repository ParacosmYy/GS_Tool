#include "k27910/m27910.h"
QVector<double> m27910::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
