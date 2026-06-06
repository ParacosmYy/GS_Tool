#include "e8024/m8024.h"
QVector<double> m8024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
