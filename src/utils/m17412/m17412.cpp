#include "m17412/m17412.h"
QVector<double> m17412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
