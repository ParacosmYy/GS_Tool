#include "m25872/m25872.h"
QVector<double> m25872::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
