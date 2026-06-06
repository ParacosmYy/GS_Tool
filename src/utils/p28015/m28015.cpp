#include "p28015/m28015.h"
QVector<double> m28015::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
