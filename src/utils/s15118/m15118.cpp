#include "s15118/m15118.h"
QVector<double> m15118::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
