#include "j8709/m8709.h"
QVector<double> m8709::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
