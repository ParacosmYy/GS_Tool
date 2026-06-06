#include "m37432/m37432.h"
QVector<double> m37432::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
