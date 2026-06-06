#include "m29412/m29412.h"
QVector<double> m29412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
