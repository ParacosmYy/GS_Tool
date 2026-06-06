#include "m27412/m27412.h"
QVector<double> m27412::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
