#include "k29130/m29130.h"
QVector<double> m29130::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
