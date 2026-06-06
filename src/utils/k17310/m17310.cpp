#include "k17310/m17310.h"
QVector<double> m17310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
