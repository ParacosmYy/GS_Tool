#include "a15220/m15220.h"
QVector<double> m15220::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
