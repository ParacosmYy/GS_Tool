#include "g8566/m8566.h"
QVector<double> m8566::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
