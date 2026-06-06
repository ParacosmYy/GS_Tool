#include "e36024/m36024.h"
QVector<double> m36024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
