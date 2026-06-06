#include "j8209/m8209.h"
QVector<double> m8209::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
