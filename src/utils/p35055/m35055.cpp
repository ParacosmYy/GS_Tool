#include "p35055/m35055.h"
QVector<double> m35055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
