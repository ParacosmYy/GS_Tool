#include "g8546/m8546.h"
QVector<double> m8546::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
