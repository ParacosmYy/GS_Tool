#include "g8106/m8106.h"
QVector<double> m8106::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
