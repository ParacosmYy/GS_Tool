#include "g8806/m8806.h"
QVector<double> m8806::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
