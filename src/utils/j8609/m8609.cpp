#include "j8609/m8609.h"
QVector<double> m8609::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
