#include "h8107/m8107.h"
QVector<double> m8107::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
