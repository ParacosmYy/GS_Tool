#include "m17852/m17852.h"
QVector<double> m17852::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
