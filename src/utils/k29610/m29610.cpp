#include "k29610/m29610.h"
QVector<double> m29610::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
