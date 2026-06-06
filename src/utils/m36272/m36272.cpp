#include "m36272/m36272.h"
QVector<double> m36272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
