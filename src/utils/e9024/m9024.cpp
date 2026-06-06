#include "e9024/m9024.h"
QVector<double> m9024::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
