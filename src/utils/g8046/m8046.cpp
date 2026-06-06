#include "g8046/m8046.h"
QVector<double> m8046::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
