#include "i20488/m20488.h"
QVector<double> m20488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
