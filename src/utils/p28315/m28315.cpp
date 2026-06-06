#include "p28315/m28315.h"
QVector<double> m28315::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
