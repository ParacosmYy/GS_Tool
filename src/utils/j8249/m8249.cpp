#include "j8249/m8249.h"
QVector<double> m8249::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
