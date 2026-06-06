#include "g8706/m8706.h"
QVector<double> m8706::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
