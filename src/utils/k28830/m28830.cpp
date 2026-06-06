#include "k28830/m28830.h"
QVector<double> m28830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
