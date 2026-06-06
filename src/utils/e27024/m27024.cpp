#include "e27024/m27024.h"
QVector<double> m27024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
