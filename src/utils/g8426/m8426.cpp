#include "g8426/m8426.h"
QVector<double> m8426::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
